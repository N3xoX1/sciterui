
class TextTapeBody extends Element {
  
  mmt; 
  nItems = 0;

  constructor(props) {
    super();
    this.mmt = props.mmt;
    this.nItems = this.mmt.totalLines();
  }

  // scroll down
  appendElements(index, n) {
          if (index === undefined) index = 0;
          const elements = [];
          for (let i = 0; i < n; ++i, ++index) {
              if (index >= this.nItems) break;
              elements.push(this.renderItem(index));
          }

          this.append(elements);
          return {moreafter: (this.nItems - index)}; // return estimated number of items below this chunk
  }

  // scroll up
  prependElements(index, n) {
          if (index === undefined) index = this.nItems - 1;
          const elements = [];
          for (let i = 0; i < n; ++i, --index) {
              if (index < 0) break;
              elements.push(this.renderItem(index));
          }

          elements.reverse();
          this.prepend(elements);
          return {morebefore: (index < 0 ? 0 : index + 1)}; // return estimated number of items above this chunk
  }

  // scroll to
  replaceElements(index, n) {
          const elements = [];
          const start = index;
          for (let i = 0; i < n; ++i, ++index) {
              if (index >= this.nItems) break;
              elements.push(this.renderItem(index));
          }

          this.patch(elements);
          return {
              morebefore: (start <= 0 ? 0 : start),
              moreafter: (this.nItems - index),
          }; // return estimated number of items before and above this chunk
  }

  // plain text version
  renderItem(index) {
          let text = this.mmt.lineAt(index); 
          return <tr key={index}>
              <td>{ index + 1 }</td>
              <td>{ text }</td>
          </tr>;
  }

  // if D's MemoryMappedText supplies HTML for lines (e.g. syntax highlighting) 
  // then use this:
  /*
  renderItem(index) {
          let html = this.mmt.lineAt(index); 
          return <tr key={index}>
              <td>{ index + 1 }</td>
              <td state-html={html}></td>
          </tr>;
  }*/

  // behavior:virtual-list will call this to pump more data 
  oncontentrequired(evt) {
          const {length, start, where} = evt.data;

          if (where > 0)
            // scrolling down, need to append more elements
            evt.data = this.appendElements(start, length);
          else if (where < 0)
            // scrolling up, need to prepend more elements
            evt.data = this.prependElements(start, length);
          else
            // scrolling to index
            evt.data = this.replaceElements(start, length);

          return true;
  }

  render() {
    return <tbody></tbody>;
  }

}

export class TextTape extends Element {

  mmt; // memory mapped file from D

  constructor(props) {
    super();
    this.mmt = props.mmt;
  }  

  render() {
    return <table fixedlayout>
      <TextTapeBody mmt={this.mmt} />
    </table>
  }
}